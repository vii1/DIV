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
* Description:	Palette programming routines. Provides full support for
*				palette rotation and palette fading. If the current
*				video mode is a 15, 16 or 24 bit mode, we still support
*				the palette functions in order to provide color lookup
*				capabilities for programs in these modes using the
*				MGL_realColor() routine. Obviously there is no hardware
*				support, so palette rotations and palette fades will not
*				have an effect until the displays is redrawn :-)
*
*
****************************************************************************/

#include "mgl.h"				/* Private declarations					*/

/*--------------------------- Global Variables ----------------------------*/

#ifndef	MGL_LITE
uchar _VARAPI _MGL_translate[256];	/* Palette translation vector		*/
#endif

/*------------------------- Implementation --------------------------------*/

#define	TO_PAL(c)	((palette_t*)(c))

/****************************************************************************
DESCRIPTION:
Returns the real packed MGL color for a color index.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to map color with
color	- Color to map

RETURNS:
Real packed MGL color value for the color index.

REMARKS:
This function returns a packed MGL color value appropriate for the specified device
context given a color index. This routine works with all device contexts, including
RGB device contexts. For the color index devices, the value is simply returned
unchanged. However for RGB devices, the color index is translated via the current
color palette for the device to find the appropriate packed MGL color value for that
device. Thus you can still write code for RGB devices that works with color indexes
(although you cannot do things like hardware palette fades and rotates as the palette
is implemented in software).

SEE ALSO:
MGL_setColorCI, MGL_setColorRGB, MGL_setPalette, MGL_getPalette

****************************************************************************/
color_t MGLAPI MGL_realColor(
	MGLDC *dc,
	int color)
{
	if (dc->mi.maxColor > 255 || dc->a.colorMode == MGL_DITHER_RGB_MODE)
		return dc->colorTab[color];
	else return color;
}

/****************************************************************************
DESCRIPTION:
Computes a packed MGL color from a 24 bit RGB tuple.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to map color with
R	- Red component of color to map (0 - 255)
G	- Green component of color to map (0 - 255)
B	- Blue component of color to map (0 - 255)

RETURNS:
Packed MGL color closest to specified RGB tuple.

REMARKS:
This function computes the packed MGL color value from a specific 24 bit RGB
tuple for a device context. If the device context is an RGB device context or an 8 bit
device in RGB dithered mode, this value simply returns the proper packed MGL
pixel value representing this color (the same as MGL_packColor would). However
if the device context is a color index device, the color palette is searched for the
color value that is the closest to the specified color. This function allows you to
specify a color given an RGB tuple, and will work in color index modes as well
with any color palette.

SEE ALSO:
MGL_realColor, MGL_setColorCI, MGL_setColorRGB
****************************************************************************/
color_t MGLAPI MGL_rgbColor(
	MGLDC *dc,
	uchar R,
	uchar G,
	uchar B)
{
	if (dc->mi.maxColor > 255 || dc->a.colorMode == MGL_DITHER_RGB_MODE)
		return MGL_packColorFast(&dc->pf,R,G,B);
	else {
		palette_t	*pal = TO_PAL(dc->colorTab);
		uint		i;
		int			closest = 0;
		int			d,distance = 0x7FFFL;

		/* Look for closest match */
		for (i = 0; i < dc->mi.maxColor; i++,pal++) {
			if ((d = ABS((int)R - (int)pal->red)
				   + ABS((int)G - (int)pal->green)
				   + ABS((int)B - (int)pal->blue)) < distance) {
				distance = d;
				closest = i;
				}
			}
		return closest;
		}
}

/****************************************************************************
DESCRIPTION:
Sets the current foreground color given a 24 bit RGB tuple.

HEADER:
mgraph.h

PARAMETERS:
R	- Red component of color (0	- 255)
G	- Green component of color (0 - 255)
B	- Blue component of color (0 - 255)

REMARKS:
This function sets the foreground color to a specific 24 bit RGB tuple. If the device
context is an RGB device context or an 8 bit device in RGB dithered mode, this
value simply sets the proper packed MGL pixel value representing this color (the
same as MGL_packColor would). However if the device context is a color index
device, the color palette is searched for the color value that is the closest to the
specified color. This function allows you to specify a color given an RGB tuple, and
will work in color index modes as well with any color palette.

SEE ALSO:
MGL_setColor, MGL_setColorCI, MGL_rgbColor
****************************************************************************/
void MGLAPI MGL_setColorRGB(
	uchar R,
	uchar G,
	uchar B)
{
	if (DC.mi.maxColor > 255 || DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.setColor(MGL_packColorFast(&DC.pf,R,G,B));
	else
		DC.r.setColor(MGL_rgbColor(&DC,R,G,B));
}

/****************************************************************************
DESCRIPTION:
Sets the current foreground color given a color index.

HEADER:
mgraph.h

PARAMETERS:
index	- Color index of color to set

REMARKS:
Sets the current foreground color value given a color index. This routine works with
all device contexts, including RGB device contexts. For the color index devices, the
value for the foreground color is simply set unchanged. However for RGB devices,
the color index is translated via the current color palette for the device to find the
appropriate packed MGL color value for that device. Thus you can still write code
for RGB devices that works with color indexes.

SEE ALSO:
MGL_setColor, MGL_setColorRGB, MGL_realColor
****************************************************************************/
void MGLAPI MGL_setColorCI(
	int index)
{
	if (DC.mi.maxColor > 255 || DC.a.colorMode == MGL_DITHER_RGB_MODE)
		DC.r.setColor(DC.colorTab[index]);
	else
		DC.r.setColor(index);
}

/****************************************************************************
DESCRIPTION:
Sets a single palette entry.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to set palette entry in
entry	- Palette index to program
red	- Red component for palette entry
green	- Green component for palette entry
blue	- Blue component for palette entry

REMARKS:
Sets the color values of a single palette entry. If you wish to set more than a single
palette index you should use the

MGL_setPalette routine which is faster for multiple entries. Note that this routine
does not actually change the value of the hardware palette, and if you wish to
change the hardware palette to reflect the new values, you will need to call the
MGL_realizePalette function to update the hardware palette.

This function is also valid for RGB device contexts, and will simply set the color
translation tables for these devices (used for drawing color index bitmaps and
translating color index color values to RGB values).

SEE ALSO:
MGL_getPaletteEntry, MGL_setPalette, MGL_getPalette, MGL_realizePalette
****************************************************************************/
void MGLAPI MGL_setPaletteEntry(
	MGLDC *dc,
	int entry,
	uchar red,
	uchar green,
	uchar blue)
{
	if (dc->mi.maxColor > 255)
		dc->colorTab[entry] = MGL_packColor(&dc->pf,red,green,blue);
	else {
		palette_t *pal = TO_PAL(dc->colorTab) + entry;
		pal->red = red;	pal->green = green;	pal->blue = blue; pal->alpha = 0;
		}
}

/****************************************************************************
DESCRIPTION:
Returns the value of a single palette entry.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest
entry	- Palette index to read
red	- Place to store the red component
green	- Place to store the green component
blue	- Place to store the blue component

REMARKS:
This function returns the value of a single color palette entry. If you wish to obtain
more than a single palette entry you should use the MGL_getPalette routine which
is faster for multiple entries.

SEE ALSO:
MGL_setPaletteEntry, MGL_getPalette, MGL_setPalette
****************************************************************************/
void MGLAPI MGL_getPaletteEntry(
	MGLDC *dc,
	int entry,
	uchar *red,
	uchar *green,
	uchar *blue)
{
	if (dc->mi.maxColor > 255)
		MGL_unpackColor(&dc->pf,dc->colorTab[entry],red,green,blue);
	else {
		palette_t	*pal = TO_PAL(dc->colorTab) + entry;
		*red = pal->red;
		*green = pal->green;
		*blue = pal->blue;
		}
}

/****************************************************************************
DESCRIPTION:
Sets the palette values for a device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to set palette values for
pal	- Pointer to array of palette values to set
numColors	- Number of color values to set
startIndex	- Starting index of first color value to set

REMARKS:
This function sets part or all of the color palette for the device context. You can
specify only a subset of the palette values to be modified with the startIndex and
numColors arguments. Thus:

	MGL_setPalette(dc,pal,10,50);

will program the 10 color indices from 50-60 with the values stored in the
palette buffer 'pal'.

Note:This routine does not actually change the value of
the hardware palette.  If you wish to change the hardware palette to reflect the
new values, you will need to call the MGL_realizePalette function to update the
hardware palette.

Note:You must ensure that you do not attempt to program invalid color indices!
Use MGL_maxColor() to find the largest color index in color index modes.

Note:This function is also valid for RGB device contexts, and will simply set the color
translation tables for these devices (used for drawing color index bitmaps and
translating color index color values to RGB values).

SEE ALSO:
MGL_getPalette, MGL_setPaletteEntry, MGL_realizePalette
****************************************************************************/
void MGLAPI MGL_setPalette(
	MGLDC *dc,
	palette_t *pal,
	int numColors,
	int startIndex)
{
	int			i;
	palette_t	*p;

	if (dc->mi.maxColor > 255) {
		for (i = startIndex,pal += startIndex; i < startIndex + numColors; i++,pal++)
			dc->colorTab[i] = MGL_packColor(&dc->pf,pal->red,pal->green,pal->blue);
		}
	else {
		/* We cant do a memcpy() as we must ensure that the alpha values are
		 * always zero so that our palette searching code can run as
		 * efficiently as possible.
         */
    	p = TO_PAL(dc->colorTab) + startIndex;
		for (i = startIndex,pal += startIndex; i < startIndex + numColors; i++,pal++) {
			*p++ = *pal;
            pal->alpha = 0;
        	}
		}
}

/****************************************************************************
DESCRIPTION:
Returns the currently active palette values.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest
pal	- Place to store the retrieved values
numColors	- Number of color values to retrieve
startIndex	- Starting palette index value to retrieve

REMARKS:
This function copies part or all of the currently active palette values and stores it in
the array pal. You can specify only a subset of the palette values to be obtained with
the startIndex and numColors arguments.

Thus to save the entire palette in a 256 color video mode, you would use (assuming
enough space for the palette has been allocated):

	MGL_getPalette(pal,255,0);

or to get the top half of the palette you would use:

	MGL_getPalette(pal,128,128);

You should ensure that you have allocated enough memory to hold all of the palette
values that you wish to read. You can use MGL_getPaletteSize to determine the
size required to save the entire palette.

SEE ALSO:
MGL_getPaletteEntry, MGL_setPalette, MGL_getDefaultPalette
****************************************************************************/
void MGLAPI MGL_getPalette(
	MGLDC *dc,
	palette_t *pal,
	int numColors,
	int startIndex)
{
	int	i;

	if (dc->mi.maxColor > 255) {
		for (i = startIndex; i < startIndex + numColors; i++)
			MGL_unpackColor(&dc->pf,dc->colorTab[i],&pal[i].red,&pal[i].green,&pal[i].blue);
		}
	else memcpy(pal,TO_PAL(dc->colorTab) + startIndex,
			numColors * sizeof(palette_t));
}

/****************************************************************************
DESCRIPTION:
Returns the number of entries in the entire palette.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

RETURNS:
Number of entries in entire palette.

REMARKS:
This function returns the number of entries in the entire palette. You should use this
function to determine the size of the entire palette, since the palette is still available
in HiColor and TrueColor video modes. For RGB modes the palette is implemented
in software rather than hardware, and is used for translating color index values to
RGB color values, such as when displaying color index bitmaps in RGB modes.

SEE ALSO:
MGL_getPalette
****************************************************************************/
int MGLAPI MGL_getPaletteSize(
	MGLDC *dc)
{
	if (dc->mi.maxColor > 255)
		return 256;
	else return dc->mi.maxColor + 1;
}

/****************************************************************************
DESCRIPTION:
Returns the default palette for the device.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest
pal	- Place to store the default palette values

REMARKS:
Copies the default palette for the specified device context into the passed palette
structure.

Note: The size of the default palette can be found with a call to
MGL_getPaletteSize().

SEE ALSO:
MGL_setPalette, MGL_getPalette
****************************************************************************/
void MGLAPI MGL_getDefaultPalette(
	MGLDC *dc,
	palette_t *pal)
{
	void *p = dc->r.getDefaultPalette(dc);
	if (p)
		memcpy(pal,p,MGL_getPaletteSize(dc) * sizeof(palette_t));
}

/****************************************************************************
DESCRIPTION:
Realizes the hardware color palette for the display device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context to realize palette for
numColors	- Number of colors to realize
startIndex 	- Starting index of first color to realize
waitVRT	- True if routine should sync to vertical retrace, false if not.

REMARKS:
This function realizes the hardware palette associated with a display device context.
Calls to MGL_setPalette only update the palette values in the color palette for the
device context structure, but do not actually program the hardware palette for
display device contexts in 4 and 8 bits per pixel modes. In order to program the
hardware palette you must call this routine.

When the hardware palette is realized, you normally need to sync to the vertical
retrace to ensure that the palette values are programmed without the onset of snow
(see MGL_setPaletteSnowLevel to adjust the number of colors programmed per
retrace period). If however you wish to perform double buffered animation and
change the hardware color palette at the same time, you should call this routine
immediately after calling either MGL_setVisualPage or MGL_swapBuffers with
the waitVRT flag set to false.

SEE ALSO:
MGL_setPalette, MGL_setVisualPage, MGL_swapBuffers, MGL_setPaletteSnowLevel
****************************************************************************/
void MGLAPI MGL_realizePalette(
	MGLDC *dc,
	int numColors,
	int startIndex,
	int waitVRT)
{
	if (dc->mi.maxColor <= 255) {
		dc->r.realizePalette(dc,TO_PAL(dc->colorTab),numColors,
			startIndex,waitVRT);
		}
}

/****************************************************************************
DESCRIPTION:
Resets the palette to the default values.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context of interest

REMARKS:
Sets the palette to the current MGL default values for the device context. This can
be used to reset the palette to the original default values that the palette is
programmed with when MGL is initialized.

SEE ALSO:
MGL_getDefaultPalette, MGL_setPalette, MGL_getPalette
****************************************************************************/
void MGLAPI MGL_setDefaultPalette(
	MGLDC *dc)
{
	palette_t	pal[256];

	MGL_getDefaultPalette(dc,pal);
	MGL_setPalette(dc,pal,MGL_getPaletteSize(dc),0);
}

#ifndef	MGL_LITE
/****************************************************************************
DESCRIPTION:
Map the colors of a memory device context to match a new palette.

HEADER:
mgraph.h

PARAMETERS:
dc	- Memory device context to map (8 bits per pixel only)
pal	- New palette to map to

REMARKS:
This function maps the pixels of an 8 bits per pixel memory device context to the
specified palette, and then sets the palette for the device context to the new palette.
This function actually translates every pixel in the device context's surface to the
new palette, by looking for the entry in the new palette that is the closest to color of
the original pixel in the old palette (the one currently active before this routine was
called).

SEE ALSO:
MGL_setPalette
****************************************************************************/
void MGLAPI MGL_mapToPalette(
	MGLDC *dc,
	palette_t *pal)
{
	palette_t	devPal[256];

	if (MGL_surfaceAccessType(dc) == MGL_NO_ACCESS || dc->mi.bitsPerPixel != 8)
		MGL_fatalError("MGL_mapToPalette() only works on 8 bit linear access DC's");

	/* Read the current palette and use as source palette for translate */
	MGL_getPalette(dc,devPal,256,0);

	/* Set the final color palette in the bitmap */
	MGL_setPalette(dc,pal,256,0);
	MGL_realizePalette(dc,256,0,false);

	/* Translate the bitmap data in place */
	_MGL_translate8to8(dc,0,0,dc->mi.xRes+1,dc->mi.yRes+1,0,0,
		MGL_REPLACE_MODE,dc->surface,dc->mi.bytesPerLine,devPal);
}

/* {secret} */
int _MGL_closestColor(palette_t *color,palette_t *pal,int palSize)
/****************************************************************************
*
* Function:		MGL_closestColor
* Parameters:	color	- RGB color palette value to search for
*				pal		- Pointer to color lookup table to scan
*				palSize	- Size of the color lookup table
* Returns:		Color index of the closest color to the specified RGB value
*
* Description:	This routine searches a color palette for the color that
*				is the closest to the specified RGB color value.
*
****************************************************************************/
{
	int			i,closest = 0;
	int			d,distance = 0x7FFFL;

	/* Look for closest match */
	for (i = 0; i < palSize; i++,pal++) {
		if ((d = ABS((int)color->red - (int)pal->red)
			   + ABS((int)color->green - (int)pal->green)
			   + ABS((int)color->blue - (int)pal->blue)) < distance) {
			distance = d;
			closest = i;
			}
		}
	return closest;
}
#endif

#if defined(NO_ASSEMBLER) || !defined(__INTEL__)

/****************************************************************************
DESCRIPTION:
Rotates the palette values for a device context.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context containing palette
numColors	- Number of colors to rotate
startIndex 	- Starting index for colors to rotate
direction	- Direction to rotate the palette entries

REMARKS:
This function rotates the palette values in the device context in the specified
direction. Note that this routine does not effect the currently active hardware
palette, and you must call MGL_realizePalette in order to make the program the
rotated palette to the hardware.

Supported directions of rotation are enumerated in MGL_palRotateType.

When the direction specified is MGL_ROTATE_UP, the first entry in the palette is
moved to the last position in the palette, and all the remaining entries are moved
one position up in the array. If the direction specified is MGL_ROTATE_DOWN,
the last entry is moved into the first entry of the palette, and the remaining entries
are all moved one position down in the array.

SEE ALSO:
MGL_setPalette, MGL_getPalette, MGL_fadePalette
****************************************************************************/
void ASMAPI MGL_rotatePalette(
	MGLDC *dc,
	int numColors,
	int startIndex,
	int direction)
{
	palette_t	*p,t;

	numColors--;
	if (direction == MGL_ROTATE_DOWN) {
		p = &TO_PAL(dc->colorTab)[startIndex];
		t = p[numColors];
		memmove(p+1,p,numColors*sizeof(palette_t));
		p[0] = t;
		}
	else {
		p = &TO_PAL(dc->colorTab)[startIndex];
		t = p[0];
		memmove(p,p+1,numColors*sizeof(palette_t));
		p[numColors] = t;
		}
}

/****************************************************************************
DESCRIPTION:
Fades the values for a color palette.

HEADER:
mgraph.h

PARAMETERS:
dc	- Device context with palette to fade
fullIntensity	- Pointer to full intensity palette to fade from
numColors	- Number of colors in palette to fade
startIndex	- Starting index of first color to fade
intensity	- Intensity for the final output palette (0	- 255)

RETURNS:
True if the entire output palette is black, false if not.

REMARKS:
This routine will take the values from a full intensity palette_t structure, fade the
values and store them into a device context palette. The actual hardware palette will
not be programmed at this stage, so you will then need to make a call to
MGL_realizePalette to make the changes visible.

The intensity value is a number between 0 and 255 that defines the intensity of the
output values. An intensity of 255 will produce the same output values as the input
values. An intensity of 128 will product values in the output palette that are half the
intensity of the input palette and an intensity of 0 produces an all black palette.

If the entire output palette is black, then the routine will return true, otherwise it will
return false.

SEE ALSO:
MGL_setPalette, MGL_getPalette, MGL_rotatePalette, MGL_realizePalette
****************************************************************************/
ibool ASMAPI MGL_fadePalette(
	MGLDC *dc,
	palette_t *fullIntensity,
	int numColors,
	int startIndex,
	uchar intensity)
{
	uchar	allBlack = 0,*p,*fi;
	int		i;

	p = (uchar*)&TO_PAL(dc->colorTab)[startIndex];
	fi = (uchar*)fullIntensity;
	for (i = 0; i < numColors; i++) {
		*p = (*fi++ * intensity) / (uchar)255;
		allBlack |= *p++;
		*p = (*fi++ * intensity) / (uchar)255;
		allBlack |= *p++;
		*p = (*fi++ * intensity) / (uchar)255;
		allBlack |= *p++;
		fi++; p++;
		}
	return true;
}

#endif

