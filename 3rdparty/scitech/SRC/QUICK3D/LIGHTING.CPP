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
* Description:  3D lighting routines.
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

inline fix32_t convertFX(real component)
{
	return (component > REAL(1) ? MGL_TOFIX(255)
		: FXrealToFixed(FXmul(REAL(255),component) + REAL(0.5)));
}

MGLLight::MGLLight()
/****************************************************************************
*
* Function:		MGLLight::MGLLight
*
* Description:	Setup the default values for a light source.
*
****************************************************************************/
{
	ambient = FXColor(0,0,0);
	diffuse = specular = FXColor(REAL(1),REAL(1),REAL(1));
	dir = FXVec3d(0,0,REAL(-1));
}

MGLMaterial::MGLMaterial()
/****************************************************************************
*
* Function:		MGLMaterial::MGLMaterial
*
* Description:	Setup the default values for a surface material.
*
****************************************************************************/
{
	ambient = FXColor(REAL(0.2),REAL(0.2),REAL(0.2));
	diffuse = FXColor(REAL(0.8),REAL(0.8),REAL(0.8));
	specular = FXColor(0,0,0);
	emission = FXColor(0,0,0);
	shininess = 0;
	ambientIndex = 0;
	diffuseIndex = REAL(1);
	specularIndex = REAL(1);
	shadeTab = NULL;
}

MGLMaterial::~MGLMaterial()
{
	delete [] shadeTab;
}

void MGLMaterial::buildRamp(palette_t *pal)
/****************************************************************************
*
* Function:		MGLMaterial::buildRamp
* Parameters:	pal	- Palette to put ramp values into
*
* Description:	Builds the color ramp from the material information and
*				stores in the passed color map.
*
****************************************************************************/
{
	int		i;
	real	dist;
	FXColor	c,spec = diffuse + specular;

	int aIndex = FXrealToInt(ambientIndex);
	int dIndex = FXrealToInt(diffuseIndex);
	int sIndex = FXrealToInt(specularIndex);
	dist = FXintToReal(dIndex - aIndex);
	for (i = aIndex; i <= dIndex; i++) {
		interpolate(c,ambient,diffuse,FXdiv(FXintToReal(i - aIndex),dist));
		pal[i].red = convert(c.r);
		pal[i].green = convert(c.g);
		pal[i].blue = convert(c.b);
		}
	dist = FXintToReal(sIndex - dIndex);
	for (i = dIndex+1; i <= sIndex; i++) {
		interpolate(c,diffuse,spec,FXdiv(FXintToReal(i - dIndex),dist));
		pal[i].red = convert(c.r);
		pal[i].green = convert(c.g);
		pal[i].blue = convert(c.b);
		}
}

void MGLMaterial::buildShadeTable(MGLDevCtx& dc)
/****************************************************************************
*
* Function:		MGLMaterial::buildShadeTable
*
* Description:	Allocates and builds the shade table from the material
*				information for fast shading in TrueColor modes. The size
*				of the shade table is determined by the number of indices
*               to be used by the material. Note that we re-set the ambient
*				index of the material to 0 for use with shade table
*				operations.
*
****************************************************************************/
{
	int		i;
	real	dist;
	FXColor	c,spec = diffuse + specular;

	// Reset the ambient index to zero and re-adjust all other indices
	diffuseIndex -= ambientIndex;
	specularIndex -= ambientIndex;
	ambientIndex = 0;

	int dIndex = FXrealToInt(diffuseIndex);
	int sIndex = FXrealToInt(specularIndex);

	// Allocate the memory for the shade table
	shadeTab = new color_t[sIndex+1];

	dist = FXintToReal(dIndex);
	for (i = 0; i <= dIndex; i++) {
		interpolate(c,ambient,diffuse,FXdiv(FXintToReal(i),dist));
		shadeTab[i] = dc.packColorFast(convert(c.r),convert(c.g),convert(c.b));
		}
	dist = FXintToReal(sIndex - dIndex);
	for (i = dIndex+1; i <= sIndex; i++) {
		interpolate(c,diffuse,spec,FXdiv(FXintToReal(i - dIndex),dist));
		shadeTab[i] = dc.packColorFast(convert(c.r),convert(c.g),convert(c.b));
		}
}

void Quick3d::addLight(MGLLight *light)
/****************************************************************************
*
* Function:		Quick3d::addLight
* Parameters:	light	- Light to add to the scene
* Returns:		True if light was added, false if too many lights
*
* Description:	Adds the light to the set of scene lights. Note that
*				we transform the light via the current ModelView matrix,
*				and we also negate it's value to simplify the code
*				required to perform the lighting calculations.
*
****************************************************************************/
{
	if (numLights >= maxLights)
		MGL_fatalError("Attempt to add too many lights to Quick3d...");
	MGLLight *l = &lights[numLights++];
	*l = *light;

	// Should be mapping by the transpose ModelView inverse!
	modelView.map(l->dir,light->dir);
	l->dir.normalise();
	l->dir.negate();
	l->sdir = l->dir;
	l->sdir.z += REAL(1);
	l->sdir.normalise();
	if (!rgbFlag) {
		// Pre-compute color index scale factors for the diffuse and
		// specular components
		l->diffuse.r =  FXmul(REAL(0.30),l->diffuse.r) +
						FXmul(REAL(0.59),l->diffuse.g) +
						FXmul(REAL(0.11),l->diffuse.b);
		l->specular.r = FXmul(REAL(0.30),l->specular.r) +
						FXmul(REAL(0.59),l->specular.g) +
						FXmul(REAL(0.11),l->specular.b);
		}
}

inline void Quick3d::addLightRGB(FXColor& c,const FXVec3d& normal,
	MGLLight *light,MGLMaterial *mat)
/****************************************************************************
*
* Function:		Quick3d::addLightRGB
*
* Description:	Adds the contribution from a single light source in RGB
*				modes.
*
****************************************************************************/
{
	real dot = normal % light->dir;
	if (dot > 0) {
		c.r += FXmul(FXmul(light->diffuse.r,dot),mat->diffuse.r);
		c.g += FXmul(FXmul(light->diffuse.g,dot),mat->diffuse.g);
		c.b += FXmul(FXmul(light->diffuse.b,dot),mat->diffuse.b);
		if (mat->shininess != 0) {
			dot = FXpow(normal % light->sdir,mat->shininess);
			c.r += FXmul(FXmul(light->specular.r,dot),mat->specular.r);
			c.g += FXmul(FXmul(light->specular.g,dot),mat->specular.g);
			c.b += FXmul(FXmul(light->specular.b,dot),mat->specular.b);
			}
		}
}

inline void Quick3d::addLightIndex(real &diffuse,real &specular,
	const FXVec3d& normal,MGLLight *light,MGLMaterial *mat)
/****************************************************************************
*
* Function:		Quick3d::addLightIndex
*
* Description:	Adds the contribution from a single light source in color
*				index modes.
*
****************************************************************************/
{
	real dot = normal % light->dir;
	if (dot > 0) {
		diffuse += FXmul(light->diffuse.r,dot);
		if (mat->shininess != 0) {
			dot = FXpow(normal % light->sdir,mat->shininess);
			specular += FXmul(light->specular.r,dot);
			}
		}
}

inline real Quick3d::computeIndex(real diffuse,real specular,MGLMaterial *mat)
/****************************************************************************
*
* Function:		Quick3d::computeIndex
* Returns:		Color index to use
*
* Description:	Computes the color index for a vertex given the specular,
*				and diffuse parameters and the currently bound material.
*
****************************************************************************/
{
	if (specular > REAL(1))
		specular = REAL(1);
	real index = mat->ambientIndex + FXmul(FXmul(diffuse,REAL(1)-specular),
		mat->diffuseIndex - mat->ambientIndex) +
		+ FXmul(specular,mat->specularIndex - mat->ambientIndex);
	if (index > mat->specularIndex)
		index = mat->specularIndex;
	return index;
}

void Quick3d::lightFrontFlatRGB()
/****************************************************************************
*
* Function:		Quick3d::lightFrontFlatRGB
*
* Description:	Performs lighting calculations on a front face in flat
*				shaded RGB modes.
*
****************************************************************************/
{
	int			i;
	FXColor 	c;
	FXVec3d		normal(cntNormal);
	MGLMaterial	*mat = fMaterial;
	MGLLight	*light;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Start with emission and ambient lighting
	c = mat->emission;
	c.r += FXmul(ambientColor.r,mat->ambient.r);
	c.g += FXmul(ambientColor.g,mat->ambient.g);
	c.b += FXmul(ambientColor.b,mat->ambient.b);

	for (i = 0, light = lights; i < numLights; i++,light++)
		addLightRGB(c,normal,light,mat);
	setColor(packColorFast(convert(c.r),convert(c.g),convert(c.b)));
}

void Quick3d::lightBackFlatRGB()
/****************************************************************************
*
* Function:		Quick3d::lightBackFlatRGB
*
* Description:	Performs lighting calculations on a back face in flat
*				shaded RGB modes. The calculations are the same expect that
*				we negate the normal and use the back face material
*				properties.
*
****************************************************************************/
{
	int			i;
	FXColor 	c;
	FXVec3d		normal(-cntNormal.x,-cntNormal.y,-cntNormal.z);
	MGLMaterial	*mat = bMaterial;
	MGLLight	*light;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Start with emission and ambient lighting
	c = mat->emission;
	c.r += FXmul(ambientColor.r,mat->ambient.r);
	c.g += FXmul(ambientColor.g,mat->ambient.g);
	c.b += FXmul(ambientColor.b,mat->ambient.b);

	for (i = 0, light = lights; i < numLights; i++,light++)
		addLightRGB(c,normal,light,mat);
	setColor(packColorFast(convert(c.r),convert(c.g),convert(c.b)));
}

void Quick3d::lightFrontFlatIndex()
/****************************************************************************
*
* Function:		Quick3d::lightFrontFlatIndex
*
* Description:	Performs lighting calculations on a front face in flat
*				shaded color index modes.
*
****************************************************************************/
{
	int			i;
	FXVec3d		normal(cntNormal);
	MGLMaterial	*mat = fMaterial;
	MGLLight	*light;
	real		diffuse = 0,specular = 0;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Sum diffuse and specular components from each light source
	for (i = 0, light = lights; i < numLights; i++,light++)
		addLightIndex(diffuse,specular,normal,light,mat);

    // Now compute color index
    color_t index = FXrealToInt(computeIndex(diffuse,specular,mat));
	setColor(cmapFlag ? index : fMaterial->shadeTab[index]);
}

void Quick3d::lightBackFlatIndex()
/****************************************************************************
*
* Function:		Quick3d::lightBackFlatIndex
*
* Description:	Performs lighting calculations on a front face in flat
*				shaded color index modes.
*
****************************************************************************/
{
	int			i;
	FXVec3d		normal(-cntNormal.x,-cntNormal.y,-cntNormal.z);
	MGLMaterial	*mat = bMaterial;
	MGLLight	*light;
	real		diffuse = 0,specular = 0;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Sum diffuse and specular components from each light source
	for (i = 0, light = lights; i < numLights; i++,light++)
		addLightIndex(diffuse,specular,normal,light,mat);

	// Now compute color index
    color_t index = FXrealToInt(computeIndex(diffuse,specular,mat));
    setColor(cmapFlag ? index : bMaterial->shadeTab[index]);
}

void Quick3d::lightFrontSmoothRGB()
/****************************************************************************
*
* Function:		Quick3d::lightFrontSmoothRGB
*
* Description:	Performs lighting calculations on a front face in Smooth
*				shaded RGB modes.
*
****************************************************************************/
{
	int			i,j;
	FXColor 	c,cbase;
	pipeEntry	*e;
	MGLMaterial	*mat = fMaterial;
	MGLLight	*light;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Start with emission and ambient lighting
	cbase = mat->emission;
	cbase.r += FXmul(ambientColor.r,mat->ambient.r);
	cbase.g += FXmul(ambientColor.g,mat->ambient.g);
	cbase.b += FXmul(ambientColor.b,mat->ambient.b);

	// Compute color separately for each vertex
	for (i = 0,e = pipe; i < count; i++,e++) {
		c = cbase;
		for (j = 0, light = lights; j < numLights; j++,light++)
			addLightRGB(c,e->normal,light,mat);
		e->win.c.r = convertFX(c.r);
		e->win.c.g = convertFX(c.g);
		e->win.c.b = convertFX(c.b);
		}
}

void Quick3d::lightBackSmoothRGB()
/****************************************************************************
*
* Function:		Quick3d::lightBackSmoothRGB
*
* Description:	Performs lighting calculations on a back face in Smooth
*				shaded RGB modes. The calculations are the same expect that
*				we negate the normal and use the back face material
*				properties.
*
****************************************************************************/
{
	int			i,j;
	FXColor 	c,cbase;
	pipeEntry	*e;
	MGLMaterial	*mat = bMaterial;
	MGLLight	*light;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Start with emission and ambient lighting
	cbase = mat->emission;
	cbase.r += FXmul(ambientColor.r,mat->ambient.r);
	cbase.g += FXmul(ambientColor.g,mat->ambient.g);
	cbase.b += FXmul(ambientColor.b,mat->ambient.b);

	// Compute color separately for each vertex
	for (i = 0,e = pipe; i < count; i++,e++) {
		FXVec3d normal(-e->normal.x,-e->normal.y,-e->normal.z);
		c = cbase;
		for (j = 0, light = lights; j < numLights; j++,light++)
			addLightRGB(c,normal,light,mat);
		e->win.c.r = convertFX(c.r);
		e->win.c.g = convertFX(c.g);
		e->win.c.b = convertFX(c.b);
		}
}

void Quick3d::lightFrontSmoothIndex()
/****************************************************************************
*
* Function:		Quick3d::lightFrontSmoothIndex
*
* Description:	Performs lighting calculations on a front face in Smooth
*				shaded color index modes.
*
****************************************************************************/
{
	int			i,j;
	pipeEntry	*e;
	MGLMaterial	*mat = fMaterial;
	MGLLight	*light;
	real		diffuse,specular;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Compute color separately for each vertex
	for (i = 0,e = pipe; i < count; i++,e++) {
		diffuse = specular = 0;
		// Sum diffuse and specular components from each light source
		for (j = 0, light = lights; j < numLights; j++,light++)
			addLightIndex(diffuse,specular,e->normal,light,mat);
		e->win.c.b = FXrealToFixed(computeIndex(diffuse,specular,mat));
		}
}

void Quick3d::lightBackSmoothIndex()
/****************************************************************************
*
* Function:		Quick3d::lightBackSmoothIndex
*
* Description:	Performs lighting calculations on a front face in Smooth
*				shaded color index modes.
*
****************************************************************************/
{
	int			i,j;
	pipeEntry	*e;
	MGLMaterial	*mat = bMaterial;
	MGLLight	*light;
	real		diffuse,specular;

	if (!mat)
		MGL_fatalError("You must bind a material before you can use lighting!");

	// Compute color separately for each vertex
	for (i = 0,e = pipe; i < count; i++,e++) {
		FXVec3d	normal(-e->normal.x,-e->normal.y,-e->normal.z);
		diffuse = specular = 0;
		// Sum diffuse and specular components from each light source
		for (j = 0, light = lights; j < numLights; j++,light++)
			addLightIndex(diffuse,specular,normal,light,mat);
		e->win.c.b = FXrealToFixed(computeIndex(diffuse,specular,mat));
		}
}
